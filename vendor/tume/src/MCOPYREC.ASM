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
;** MCOPYREC.ASM
;**
;** PROGRAMMER : Daniel C. Chang
;**    VERSION : 00.000
;**    CREATED : 07/23/89
;**   MODIFIED : 07/27/94
;**       TABS : 10 18 40 44
;**
;**	     \|///-_
;**	     \oo///_
;**	-----w/-w------
;**	 E C H I D N A
;**	---------------
;**
;** DESCRIPTION
;**	Copy rect with clipping, scaling, flipping, masking, and
;**	transparencies for MCGA.
;**
;** HISTORY
;**	10/15/93 Friday (dcc) - fix scaled, Y-flipped w/ pixel mask drawn
;**		incorrectly (in MCGA_ScaledMaskedCopyYFlippedTransRect)
;**	01/28/92 Tuesday (DCC) - add _MCGA_ScaledMaskedCopyTransRect
;**	01/16/92 Thursday (DCC) - add _copyMode bit 5 (flip about Y axis)
;**		and bit 6 (flip about X axis) support
;**	01/16/92 Thursday (DCC) - add support for very large shapes
;**	11/10/91 Sunday (DCC) - add _copyMode bit 0 (don't copy transparent
;**		pixels) and bit 1 (use _pixelAND and _pixelOR masks) support
;**	05/14/91 Tuesday (DCC) - fixed so source & dest offset may be != 0
;**
	IDEAL

	JUMPS

	MACRO	MPROC PNAME

		PROC PNAME far

	ENDM

	MODEL	HUGE

	CODESEG

	PUBLIC	_copyMode
	PUBLIC	_pixelAND
	PUBLIC	_pixelOR

	PUBLIC	_wSrcSkipX
	PUBLIC	_wDstDupX
	PUBLIC	_wSrcSkipY
	PUBLIC	_wDstDupY

; MaskedCopyTransRect - copy rectangle starting at source x, y from screen
; buffer pointed to by DS: to destination x, y screen buffer pointed
; to by ES: starting at x, y of xsize, ysize.
;
; Look at the global variable _copyMode:
; _copyMode bit 0 set: don't copy transparent pixels
; _copyMode bit 1 set: mask every pixel with _pixelAND, & _pixelOR.
;
; Known bugs: doesn't do any clipping.
; by Daniel Chang, 11/25/89.
;
; INPUT:
;
; DS:SI - points to source bytemap
; ES:DI - points to destination bytemap
; AX - source x position of left edge of rectangle to copy from
; BX - source y position of top edge of rectangle to copy from
; CX - destination x position of left edge of rectangle to copy to
; DX - destination y position of top edge of rectangle to copy to
; [cs:wDstWidth] - destination width of rectangle to copy
; [cs:wDstHeight] - destination height of rectangle to copy
; [cs:sRowByte] - bytes to add to get to next row in source screen
; [cs:dRowByte] - bytes to add to get to next row in destination screen
;
; REGISTERS CHANGED: AX, BX, CX, DX, SI, DI, BP, ES, DS

	STRUC	ByteMap
Width	DW	?
Height	DW	?
gfxData	DD	?
TColor	DW	?
	ENDS

	EVEN

_copyMode db	?	;copy thru mask, use transparencies
	db	0
_pixelAND db	?	;AND pixel with this value
	db	0
_pixelOR	db	?	;OR pixel with this value
	db	0

sRowByte dw	?	;source bytemap bytes per row
sHeight	dw	?	;source bytemap number of rows
dRowByte dw	?	;destination bytemap bytes per row
wDstWidth	dw	?
wDstHeight	dw	?
dskip	dw	?	;bytes to skip to start of next row
pixTrans	dw	?

	MPROC	MCGA_MaskedCopyTransRect

	mov	bp, [cs:wDstWidth]	;save # of bytes to copy per row
	or	bp, bp
	jle	@@done	;=> width <= 0, no work
	cmp	[cs:wDstHeight], 0
	jle	@@done	;=> height <= 0, no work

	push	dx	;save dstY
	push	ax	;save srcX

	mov	ax, bx
	mul	[cs:sRowByte]	;AX = srcY * [cs:sRowByte]
	pop	bx
	add	ax, bx	;AX = srcY * [cs:sRowByte] + srcX
	add	si, ax
	jnc	@@doneMUL

	mov	ax, ds
	add	ax, 1000h
	mov	ds, ax
@@doneMUL:
	call	NormalizeDSSI

	pop	ax	;restore dstY
	mul	[cs:dRowByte]	;AX = dstY * [cs:dRowByte]
	add	ax, cx	;AX = dstY * [cs:dRowByte] + dstX
	add	di, ax
	jnc	@@donMUL

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@donMUL:
	call	NormalizeESDI

	mov	dx, [cs:sRowByte]
	sub	dx, bp

	mov	ax, [cs:dRowByte]
	sub	ax, bp
	mov	[cs:dskip], ax	;bytes to skip to next dest row

	mov	bx, [cs:wDstHeight]	;# of rows to copy

	; Check which copy mode to use.

	mov	cl, [cs:_copyMode]
	and	cl, 011b
	jz	@@copyAll	;=> copy everything
	dec	cl
	jz	@@copyTrans	;=> use transparency mask
	dec	cl
	jz	@@copyMask	;=> use pixel mask

; Copy through pixel mask & transparency mask.

	EVEN
@@yloop3:
	mov	cx, bp	;get number of bytes to copy
@@xloop3:
	lodsb
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip3	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	stosb
	loop	@@xloop3	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc0	;=> normalize number
@@SrcFixed0:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst0	;=> normalize number
@@DstFixed0:
	dec	bx
	jg	@@yloop3	;=> more rows to copy
@@done:
	ret

@@fixSrc0:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed0
@@fixDst0:
	call	NormalizeESDI
	jmp	SHORT @@DstFixed0

	EVEN
@@skip3:
	inc	di
	loop	@@xloop3	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc1	;=> normalize number
@@SrcFixed1:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst1	;=> normalize number
@@DstFixed1:
	dec	bx
	jg	@@yloop3	;=> more rows to copy
	ret

@@fixSrc1:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed1
@@fixDst1:
	call	NormalizeESDI
	jmp	SHORT @@DstFixed1

; Copy everything.

@@copyAll:
	shr	bp, 1	;# bytes per row / 2
	jnc	@@even	;=> even number of bytes to copy

	EVEN
@@yloop0:
	mov	cx, bp	;get number of bytes to copy
	movsb
	rep	movsw	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc2	;=> normalize number
@@SrcFixed2:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst2	;=> normalize number
@@DstFixed2:
	dec	bx
	jg	@@yloop0	;=> more rows to copy
	ret

@@fixSrc2:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed2
@@fixDst2:
	call	NormalizeESDI
	jmp	SHORT @@DstFixed2

	EVEN
@@even:
	mov	cx, bp	;get number of bytes to copy
	rep	movsw	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc3	;=> normalize number
@@SrcFixed3:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst3	;=> normalize number
@@DstFixed3:
	dec	bx
	jg	@@even	;=> more rows to copy
	ret

@@fixSrc3:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed3
@@fixDst3:
	call	NormalizeESDI
	jmp	SHORT @@DstFixed3

; Copy through transparency mask.

	EVEN

@@copyTrans:
@@yloop1:
	mov	cx, bp	;get number of bytes to copy
@@xloop1:
	lodsb
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip1	;=> don't copy this pixel
	stosb
	loop	@@xloop1	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc4	;=> normalize number
@@SrcFixed4:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst4	;=> normalize number
@@DstFixed4:
	dec	bx
	jg	@@yloop1	;=> more rows to copy
	ret

@@fixSrc4:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed4
@@fixDst4:
	call	NormalizeESDI
	jmp	SHORT @@DstFixed4

	EVEN
@@skip1:
	inc	di
	loop	@@xloop1	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc5	;=> normalize number
@@SrcFixed5:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst5	;=> normalize number
@@DstFixed5:
	dec	bx
	jg	@@yloop1	;=> more rows to copy
	ret

@@fixSrc5:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed5
@@fixDst5:
	call	NormalizeESDI
	jmp	SHORT @@DstFixed5

; Copy through pixel mask.

	EVEN
@@copyMask:
@@yloop2:
	mov	cx, bp	;get number of bytes to copy
@@xloop2:
	lodsb
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	stosb
	loop	@@xloop2	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc6	;=> normalize number
@@SrcFixed6:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst6	;=> normalize number
@@DstFixed6:
	dec	bx
	jg	@@yloop2	;=> more rows to copy
	ret

@@fixSrc6:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed6
@@fixDst6:
	call	NormalizeESDI
	jmp	SHORT @@DstFixed6

	ENDP	MCGA_MaskedCopyTransRect




	MPROC	MCGA_MaskedCopyXFlippedTransRect

	mov	bp, [cs:wDstWidth]	;save # of bytes to copy per row
	or	bp, bp
	jle	@@done	;=> width <= 0, no work
	cmp	[cs:wDstHeight], 0
	jle	@@done	;=> height <= 0, no work

	push	dx	;save dstY
	push	ax	;save srcX

	mov	ax, bx
	mul	[cs:sRowByte]	;AX = srcY * [cs:sRowByte]
	pop	bx
	add	ax, bx	;AX = srcY * [cs:sRowByte] + srcX
	add	si, ax
	jnc	@@doneMUL

	mov	ax, ds
	add	ax, 1000h
	mov	ds, ax
@@doneMUL:
	call	NormalizeDSSI

	pop	ax	;restore dstY
	mul	[cs:dRowByte]	;AX = dstY * [cs:dRowByte]
	add	ax, cx	;AX = dstY * [cs:dRowByte] + dstX
	add	di, ax
	jnc	@@donMUL

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@donMUL:
	call	NormalizeESDI

	add	di, bp	;DI += [cs:wDstWidth]
	dec	di	;DI -= 1

	mov	dx, [cs:sRowByte]
	sub	dx, bp

	mov	ax, [cs:dRowByte]
	add	ax, bp
	mov	[cs:dskip], ax	;bytes to skip to next dest row

	mov	bx, [cs:wDstHeight]	;# of rows to copy

	; Check which copy mode to use.

	mov	cl, [cs:_copyMode]
	and	cl, 011b
	jz	@@copyAll	;=> copy everything
	dec	cl
	jz	@@copyTrans	;=> use transparency mask
	dec	cl
	jz	@@copyMask	;=> use pixel mask

; Copy through pixel mask & transparency mask.

	EVEN
@@yloop3:
	mov	cx, bp	;get number of bytes to copy
@@xloop3:
	lodsb
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip3	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
@@skip3:
	dec	di
	loop	@@xloop3	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc0	;=> normalize number
@@SrcFixed0:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst0	;=> normalize number
@@DstFixed0:
	dec	bx
	jg	@@yloop3	;=> more rows to copy
@@done:
	ret

@@fixSrc0:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed0
@@fixDst0:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed0

; Copy everything.

	EVEN
@@copyAll:
@@yloop0:
	mov	cx, bp	;get number of bytes to copy
@@xloop0:
	lodsb
	mov	[es:di], al
	dec	di
	loop	@@xloop0	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc2	;=> normalize number
@@SrcFixed2:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst2	;=> normalize number
@@DstFixed2:
	dec	bx
	jg	@@yloop0	;=> more rows to copy
	ret

@@fixSrc2:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed2
@@fixDst2:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed2

; Copy through transparency mask.

	EVEN
@@copyTrans:
@@yloop1:
	mov	cx, bp	;get number of bytes to copy
@@xloop1:
	lodsb
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip1	;=> don't copy this pixel
	mov	[es:di], al
@@skip1:
	dec	di
	loop	@@xloop1	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc4	;=> normalize number
@@SrcFixed4:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst4	;=> normalize number
@@DstFixed4:
	dec	bx
	jg	@@yloop1	;=> more rows to copy
	ret

@@fixSrc4:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed4
@@fixDst4:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed4

; Copy through pixel mask.

	EVEN
@@copyMask:
@@yloop2:
	mov	cx, bp	;get number of bytes to copy
@@xloop2:
	lodsb
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
	dec	di
	loop	@@xloop2	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc6	;=> normalize number
@@SrcFixed6:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst6	;=> normalize number
@@DstFixed6:
	dec	bx
	jg	@@yloop2	;=> more rows to copy
	ret

@@fixSrc6:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed6
@@fixDst6:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed6

	ENDP	MCGA_MaskedCopyXFlippedTransRect




	MPROC	MCGA_MaskedCopyYFlippedTransRect

	mov	bp, [cs:wDstWidth]	;save # of bytes to copy per row
	or	bp, bp
	jle	@@done	;=> width <= 0, no work
	cmp	[cs:wDstHeight], 0
	jle	@@done	;=> height <= 0, no work

	push	dx	;save dstY
	push	ax	;save srcX

	mov	ax, bx
	mul	[cs:sRowByte]	;AX = srcY * [cs:sRowByte]
	pop	bx
	add	ax, bx	;AX = srcY * [cs:sRowByte] + srcX
	add	si, ax
	jnc	@@doneMUL

	mov	ax, ds
	add	ax, 1000h
	mov	ds, ax
@@doneMUL:
	call	NormalizeDSSI

	pop	ax	;restore dstY
	mul	[cs:dRowByte]	;AX = dstY * [cs:dRowByte]
	add	ax, cx	;AX = dstY * [cs:dRowByte] + dstX
	add	di, ax
	jnc	@@donMUL

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@donMUL:
	mov	ax, [cs:wDstHeight]
	dec	ax
	mul	[cs:dRowByte]	;AX = ([cs:wDstHeight]-1) * [cs:dRowByte]
	add	di, ax
	jnc	@@dontBump

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@dontBump:
	call	UnNormalizeESDI

	mov	dx, [cs:sRowByte]
	sub	dx, bp

	mov	ax, [cs:dRowByte]
	add	ax, bp
	mov	[cs:dskip], ax	;bytes to skip to prev dest row

	mov	bx, [cs:wDstHeight]	;# of rows to copy

	; Check which copy mode to use.

	mov	cl, [cs:_copyMode]
	and	cl, 011b
	jz	@@copyAll	;=> copy everything
	dec	cl
	jz	@@copyTrans	;=> use transparency mask
	dec	cl
	jz	@@copyMask	;=> use pixel mask

; Copy through pixel mask & transparency mask.

	EVEN
@@yloop3:
	mov	cx, bp	;get number of bytes to copy
@@xloop3:
	lodsb
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip3	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	stosb
	loop	@@xloop3	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc0	;=> normalize number
@@SrcFixed0:
	cmp	di, 1000
	jb	@@fixDst0	;=> unnormalize number
@@DstFixed0:
	sub	di, [cs:dskip]	;move to prev row in destination
	dec	bx
	jg	@@yloop3	;=> more rows to copy
@@done:
	ret

@@fixSrc0:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed0
@@fixDst0:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed0

	EVEN
@@skip3:
	inc	di
	loop	@@xloop3	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc1	;=> normalize number
@@SrcFixed1:
	cmp	di, 1000
	jb	@@fixDst1	;=> unnormalize number
@@DstFixed1:
	sub	di, [cs:dskip]	;move to prev row in destination
	dec	bx
	jg	@@yloop3	;=> more rows to copy
	ret

@@fixSrc1:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed1
@@fixDst1:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed1

; Copy everything.

@@copyAll:
	shr	bp, 1	;# bytes per row / 2
	jnc	@@even	;=> even number of bytes to copy

	EVEN
@@yloop0:
	mov	cx, bp	;get number of bytes to copy
	movsb
	rep	movsw	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc2	;=> normalize number
@@SrcFixed2:
	cmp	di, 1000
	jb	@@fixDst2	;=> unnormalize number
@@DstFixed2:
	sub	di, [cs:dskip]	;move to prev row in destination
	dec	bx
	jg	@@yloop0	;=> more rows to copy
	ret

@@fixSrc2:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed2
@@fixDst2:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed2

	EVEN
@@even:
	mov	cx, bp	;get number of bytes to copy
	rep	movsw	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc3	;=> normalize number
@@SrcFixed3:
	cmp	di, 1000
	jb	@@fixDst3	;=> unnormalize number
@@DstFixed3:
	sub	di, [cs:dskip]	;move to prev row in destination
	dec	bx
	jg	@@even	;=> more rows to copy
	ret

@@fixSrc3:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed3
@@fixDst3:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed3

; Copy through transparency mask.

	EVEN

@@copyTrans:
@@yloop1:
	mov	cx, bp	;get number of bytes to copy
@@xloop1:
	lodsb
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip1	;=> don't copy this pixel
	stosb
	loop	@@xloop1	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc4	;=> normalize number
@@SrcFixed4:
	cmp	di, 1000
	jb	@@fixDst4	;=> unnormalize number
@@DstFixed4:
	sub	di, [cs:dskip]	;move to prev row in destination
	dec	bx
	jg	@@yloop1	;=> more rows to copy
	ret

@@fixSrc4:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed4
@@fixDst4:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed4

	EVEN
@@skip1:
	inc	di
	loop	@@xloop1	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc5	;=> normalize number
@@SrcFixed5:
	cmp	di, 1000
	jb	@@fixDst5	;=> unnormalize number
@@DstFixed5:
	sub	di, [cs:dskip]	;move to prev row in destination
	dec	bx
	jg	@@yloop1	;=> more rows to copy
	ret

@@fixSrc5:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed5
@@fixDst5:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed5

; Copy through pixel mask.

	EVEN
@@copyMask:
@@yloop2:
	mov	cx, bp	;get number of bytes to copy
@@xloop2:
	lodsb
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	stosb
	loop	@@xloop2	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc6	;=> normalize number
@@SrcFixed6:
	cmp	di, 1000
	jb	@@fixDst6	;=> unnormalize number
@@DstFixed6:
	sub	di, [cs:dskip]	;move to prev row in destination
	dec	bx
	jg	@@yloop2	;=> more rows to copy
	ret

@@fixSrc6:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed6
@@fixDst6:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed6

	ENDP	MCGA_MaskedCopyYFlippedTransRect




	MPROC	MCGA_MaskedCopyXYFlippedTransRect

	mov	bp, [cs:wDstWidth]	;save # of bytes to copy per row
	or	bp, bp
	jle	@@done	;=> width <= 0, no work
	cmp	[cs:wDstHeight], 0
	jle	@@done	;=> height <= 0, no work

	push	dx	;save dstY
	push	ax	;save srcX

	mov	ax, bx
	mul	[cs:sRowByte]	;AX = srcY * [cs:sRowByte]
	pop	bx
	add	ax, bx	;AX = srcY * [cs:sRowByte] + srcX
	add	si, ax
	jnc	@@doneMUL

	mov	ax, ds
	add	ax, 1000h
	mov	ds, ax
@@doneMUL:
	call	NormalizeDSSI

	pop	ax	;restore dstY
	mul	[cs:dRowByte]	;AX = dstY * [cs:dRowByte]
	add	ax, cx	;AX = dstY * [cs:dRowByte] + dstX
	add	ax, bp	;AX = dstY * [cs:dRowByte] + dstX + [cs:wDstWidth]
	dec	ax	;AX = dstY * [cs:dRowByte] + dstX + [cs:wDstWidth]-1
	add	di, ax
	jnc	@@donMUL

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@donMUL:
	mov	ax, [cs:wDstHeight]
	dec	ax
	mul	[cs:dRowByte]	;AX = ([cs:wDstHeight]-1) * [cs:dRowByte]
	add	di, ax
	jnc	@@dontBump

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@dontBump:
	call	UnNormalizeESDI

	mov	dx, [cs:sRowByte]
	sub	dx, bp

	mov	ax, [cs:dRowByte]
	sub	ax, bp
	mov	[cs:dskip], ax	;bytes to skip to prev dest row

	mov	bx, [cs:wDstHeight]	;# of rows to copy

	; Check which copy mode to use.

	mov	cl, [cs:_copyMode]
	and	cl, 011b
	jz	@@copyAll	;=> copy everything
	dec	cl
	jz	@@copyTrans	;=> use transparency mask
	dec	cl
	jz	@@copyMask	;=> use pixel mask

; Copy through pixel mask & transparency mask.

	EVEN
@@yloop3:
	mov	cx, bp	;get number of bytes to copy
@@xloop3:
	lodsb
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip3	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
@@skip3:
	dec	di
	loop	@@xloop3	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc0	;=> normalize number
@@SrcFixed0:
	cmp	di, 1000
	jb	@@fixDst0	;=> unnormalize number
@@DstFixed0:
	sub	di, [cs:dskip]	;move to prev row in destination
	dec	bx
	jg	@@yloop3	;=> more rows to copy
@@done:
	ret

@@fixSrc0:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed0
@@fixDst0:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed0

; Copy everything.

	EVEN
@@copyAll:
@@yloop0:
	mov	cx, bp	;get number of bytes to copy
@@xloop0:
	lodsb
	mov	[es:di], al
	dec	di
	loop	@@xloop0	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc2	;=> normalize number
@@SrcFixed2:
	cmp	di, 1000
	jb	@@fixDst2	;=> unnormalize number
@@DstFixed2:
	sub	di, [cs:dskip]	;move to prev row in destination
	dec	bx
	jg	@@yloop0	;=> more rows to copy
	ret

@@fixSrc2:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed2
@@fixDst2:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed2

; Copy through transparency mask.

	EVEN
@@copyTrans:
@@yloop1:
	mov	cx, bp	;get number of bytes to copy
@@xloop1:
	lodsb
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip1	;=> don't copy this pixel
	mov	[es:di], al
@@skip1:
	dec	di
	loop	@@xloop1	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc4	;=> normalize number
@@SrcFixed4:
	cmp	di, 1000
	jb	@@fixDst4	;=> unnormalize number
@@DstFixed4:
	sub	di, [cs:dskip]	;move to prev row in destination
	dec	bx
	jg	@@yloop1	;=> more rows to copy
	ret

@@fixSrc4:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed4
@@fixDst4:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed4

; Copy through pixel mask.

	EVEN
@@copyMask:
@@yloop2:
	mov	cx, bp	;get number of bytes to copy
@@xloop2:
	lodsb
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
	dec	di
	loop	@@xloop2	;copy row

	add	si, dx	;move to next row in source
	js	@@fixSrc6	;=> normalize number
@@SrcFixed6:
	cmp	di, 1000
	jb	@@fixDst6	;=> unnormalize number
@@DstFixed6:
	sub	di, [cs:dskip]	;move to prev row in destination
	dec	bx
	jg	@@yloop2	;=> more rows to copy
	ret

@@fixSrc6:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed6
@@fixDst6:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed6

	ENDP	MCGA_MaskedCopyXYFlippedTransRect




NormalizeDSSI:

	; Normalize DS:SI. Preserves all registers.

	push	ax
	push	bx
	mov	ax, si
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	mov	bx, ds
	add	ax, bx
	mov	ds, ax
	and	si, 0Fh
	pop	bx
	pop	ax
	ret

NormalizeESDI:

	; Normalize ES:DI. Preserves all registers.

	push	ax
	push	bx
	mov	ax, di
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	mov	bx, es
	add	ax, bx
	mov	es, ax
	and	di, 0Fh
	pop	bx
	pop	ax
	ret

UnNormalizeES_1DI:

	; Subtract 100H from ES then unnormalize ESDI.

	push	ax
	mov	ax, es
	sub	ax, 100h
	mov	es, ax
	pop	ax

UnNormalizeESDI:

	; Unnormalize ES:DI. Make DI as large as possible <= 7FFFh.
	; Preserves all registers.

	push	ax
	push	bx
	or	di, di
	js	@@NegativeDI

	mov	ax, di
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	xor	ax, 07FFh
	mov	bx, es
	sub	bx, ax
	mov	es, bx
	or	di, 07FF0h
	pop	bx
	pop	ax
	ret

@@NegativeDI:
	mov	ax, di
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	shr	ax, 1
	sub	ax, 07FFh
	mov	bx, es
	add	bx, ax
	mov	es, bx
	and	di, 07FFFh
	or	di, 07FF0h
	pop	bx
	pop	ax
	ret

	
;=====================================================================
;**
;** _MCGA_MaskedCopyTransRect
;**
;** PURPOSE
;**	C glue for copy rect as above.  C Prototype is
;**
;**	MCGA_MaskedCopyTransRect (
;**		ByteMap	*src,
;**		short	 fromx,
;**		short	 fromy,
;**		ByteMap	*dest,
;**		short	 tox,
;**		short	 toy,
;**		short	 width,
;**		short	 height
;**		)
;**
;**	This routine looks at the global variable _copyMode:
;**	_copyMode bit 6 set: x flip tile
;**	_copyMode bit 5 set: y flip tile
;**	Thus, if bit 5 is set, MCGA_MaskedCopyYFlippedTransRect is called,
;**	if bit 6 is set, MCGA_MaskedCopyXFlippedTransRect is called,
;**	if bit 5 & 6 are set, MCGA_MaskedCopyXYFlippedTransRect is called,
;**	else MCGA_MaskedCopyTransRect is called.
;**
;** INPUT
;**	
;**
;** REGISTER USED
;**	
;**
;** EFFECTS
;**	
;**
;** RETURN VALUE
;**	
;**
;** LIMITS/BUGS
;**	
;**
;** HISTORY
;**	
;**

	EVEN

	PUBLIC	_MCGA_MaskedCopyTransRect
	MPROC	_MCGA_MaskedCopyTransRect
	ARG	sBMoff:WORD,sBMseg:WORD,FromX:WORD,FromY:WORD,dBMoff:WORD,dBMseg:WORD,ToX:WORD,ToY:WORD,GWidth:WORD,Height:WORD

	push	bp
	mov	bp, sp

	push	ds
	push	di
	push	si

	cld
	mov	ax, [GWidth]
	mov	[cs:wDstWidth], ax
	mov	ax, [Height]
	mov	[cs:wDstHeight], ax

	lds	si,[dword ptr dBMoff]	; Get Pointer to Dest ByteMap
	mov	ax,[(ByteMap ptr ds:si).Width]
  	mov	[cs:dRowByte],ax	; Set width of Dest ByteMap
	les	di, [(ByteMap ptr ds:si).gfxData]	;DCC Get Off, Seg of Dest ByteMap

	lds	si,[dword ptr sBMoff]	; Get Pointer to Src ByteMap

	mov	ax, [(ByteMap ptr ds:si).TColor]	;get transparent color
	mov	[cs:pixTrans], ax

	mov	ax,[(ByteMap ptr ds:si).Width]
  	mov	[cs:sRowByte],ax	; Set width of Src ByteMap
	lds	si, [(ByteMap ptr ds:si).gfxData]	;DCC Get Off, Seg of Src ByteMap

	mov	ax, [FromX]
	mov	bx, [FromY]
	mov	cx, [ToX]
	mov	dx, [ToY]

	mov	bp, [WORD PTR cs:_copyMode]
	and	bp, 01100000b

	cmp	bp, 00100000b
	jne	@@not01

	call	MCGA_MaskedCopyYFlippedTransRect
	jmp	SHORT @@done
@@not01:
	cmp	bp, 01000000b
	jne	@@not10

	call	MCGA_MaskedCopyXFlippedTransRect
	jmp	SHORT @@done
@@not10:
	cmp	bp, 01100000b
	jne	@@not11

	call	MCGA_MaskedCopyXYFlippedTransRect
	jmp	SHORT @@done
@@not11:
	call	MCGA_MaskedCopyTransRect
@@done:
	pop	si
	pop	di
	pop	ds

	pop	bp
	ret
	ENDP	_MCGA_MaskedCopyTransRect




; ScaledMaskedCopyTransRect - copy rectangle starting at source x, y from screen
; buffer pointed to by DS: to destination x, y screen buffer pointed
; to by ES: starting at x, y of xsize, ysize.
;
; Look at the global variable _copyMode:
; _copyMode bit 0 set: don't copy transparent pixels
; _copyMode bit 1 set: mask every pixel with _pixelAND, & _pixelOR.
;
; Known bugs: doesn't do any clipping.
; by Daniel Chang, 11/25/89.
;
; INPUT:
;
; DS:SI - points to source bytemap
; ES:DI - points to destination bytemap
; AX - source x position of left edge of rectangle to copy from
; BX - source y position of top edge of rectangle to copy from
; [cs:wDstWidth] - destination width of rectangle to copy
; [cs:wDstHeight] - height of source rectangle to copy
; [cs:sRowByte] - bytes to add to get to next row in source screen
; [cs:dRowByte] - bytes to add to get to next row in destination screen
; [cs:srcSkipX] - pixels to x skip in source for scaling					;+
; [cs:dstDupX] - pixels to x duplicate in destination for scaling			;+
; [cs:srcSkipY] - pixels to y skip in source for scaling					;+
; [cs:dstDupY] - pixels to y duplicate in destination for scaling			;+
; CX - wDstX / wSrcSkipX;(dest X pos in pix of left edge to copy to)		;+
; [cs:wDstModSrcSkipX] - wDstX % wSrcSkipX									;+
; [cs:wDstDivSrcSkipY] - wDstY / wSrcSkipY;(dest Y pos in pix of top edge to copy to)		;+
; [cs:wDstModSrcSkipY] - wDstY % wSrcSkipY									;+
;
; REGISTERS CHANGED: AX, BX, CX, DX, SI, DI, BP, ES, DS

	EVEN

_wSrcSkipX	dw ?
_wDstDupX	dw ?
_wSrcSkipY	dw ?
_wDstDupY	dw ?

srcSkipX	dw	?										;+
dstDupX	dw	?										;+
srcSkipY	dw	?										;+
dstDupY	dw	?										;+
												;+
wDstModSrcSkipX	dw ?										;+
wDstDivSrcSkipY	dw ?										;+
wDstModSrcSkipY	dw ?										;+
												;+
; Local variables.										;+
												;+
wModY	dw	?										;+
SIsave	dw	?										;+
wY	dw	?										;+

	MPROC	MCGA_ScaledMaskedCopyTransRect

	mov	bp, [cs:wDstWidth]	;save # of bytes to copy per row
	or	bp, bp
	jle	@@done	;=> width <= 0, no work
	cmp	[cs:wDstHeight], 0
	jle	@@done	;=> height <= 0, no work

;-	push	dx	;save dstY
	push	ax	;save srcX

	mov	ax, bx
	mul	[cs:sRowByte]	;AX = srcY * [cs:sRowByte]
	pop	bx
	add	ax, bx	;AX = srcY * [cs:sRowByte] + srcX
	add	si, ax
	jnc	@@doneMUL

	mov	ax, ds
	add	ax, 1000h
	mov	ds, ax
@@doneMUL:
	call	NormalizeDSSI

	mov	ax, [cs:wDstDivSrcSkipY]									;+
	mul	[cs:dRowByte]	;AX = (dstY/srcSkipY)*[cs:dRowByte]	;+
	add	ax, cx	;AX += (dstX / srcSkipX)			;+
	add	di, ax
	jnc	@@donMUL

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@donMUL:
	mov	ax, [cs:wDstModSrcSkipX]									;+
	cmp	ax, [cs:dstDupX]		;if (wDstX%wSrcSkipX >=wDstDupX);+
	jb	@@notIncX		;{								;+
	inc	di		;	pDst++;						;+
@@notIncX:			;}								;+
	mov	ax, [cs:wDstModSrcSkipY]									;+
	cmp	ax, [cs:dstDupY]		;if (wDstY%wSrcSkipY >=wDstDupY);+
	jb	@@notIncY		;{								;+
	add	di, [cs:dRowByte]		;	pDst += pDstBM->width;		;+
@@notIncY:			;}								;+
	call	NormalizeESDI

;-	mov	dx, [cs:sRowByte]
;-	sub	dx, bp

	mov	ax, [cs:dRowByte]
	sub	ax, [cs:wDstWidth]										;+
;-	sub	ax, bp
	mov	[cs:dskip], ax	;bytes to skip to next dest row

;-	mov	bx, [cs:wDstHeight]	;# of rows to copy
	mov	ax, [cs:wDstModSrcSkipY]									;+
	mov	[cs:wModY], ax		;wModY = wDstY % wSrcSkipY;		;+
	mov	ax, [cs:wDstHeight]										;+
	mov	[cs:wY], ax		;for (wY=0; wY<wDstHeight; )	;+

	; Check which copy mode to use.

	mov	cl, [cs:_copyMode]
	and	cl, 011b
	jz	@@copyAll	;=> copy everything
	dec	cl
	jz	@@copyTrans	;=> use transparency mask
	dec	cl
	jz	@@copyMask	;=> use pixel mask

; Copy through pixel mask & transparency mask.

	EVEN
@@yloop3:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh1:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot101		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh1										;+
	jmp	@@yendwh01										;+
@@ynot101:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh01		;{								;+
@@ydowh1:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wDstWidth; )		;+
@@xloop3:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw0		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip0	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
@@skip0:
	inc	di
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit01		;		break;
@@xnotdraw0:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx0		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx0:			;	}							;+
@@xwh01:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh01		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip3	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
@@skip3:
	inc	di
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh01		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit01		;			break;
				;	}
@@xendwh01:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop3		;}								;+
@@xexit01:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst0	;=> normalize number
@@DstFixed0:
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot111		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot111:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk011		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk011:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh1
	ret			;	break;
@@yendwh01:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot121		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot121:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk021		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk021:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc0	;=> normalize number
@@SrcFixed0:
	jmp	@@yloop3	;=> more rows to copy
@@done:
	ret

@@fixSrc0:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed0
@@fixDst0:
	call	NormalizeESDI
	jmp	SHORT @@DstFixed0

; Copy everything.

	EVEN
@@copyAll:
@@yloop0:
;-	mov	cx, bp		;get # bytes...
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh0:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot10		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh0										;+
	jmp	@@yendwh0										;+
@@ynot10:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh0		;{								;+
@@ydowh0:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop0:			;{								;+
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw		;	{							;+
;-	lodsb
	mov	al, [ds:si]		;		*pDst = *psrc;			;+
	stosb			;		pDst += wDstDX;			;+
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit02		;		break;
@@xnotdraw:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx:			;	}							;+
@@xwh0:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh0		;	{							;+
	mov	al, [ds:si]		;		*pDst = *psrc;			;+
	stosb			;		pDst += wDstDx;
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh0		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit02		;			break;
				;	}							;+
@@xendwh0:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop0		;}								;+
@@xexit02:											;+
	add	di, [cs:dskip]	;move to next row in destination	;+
	js	@@fixDst2	;=> normalize number				;+
@@DstFixed2:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot11		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot11:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk01		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk01:				;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh0
	ret			;	break;
@@yendwh0:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot12		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot12:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk02		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk02:				;}								;+
;-	add	si, dx	;move to next row in source
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc2	;=> normalize number
@@SrcFixed2:
	jmp	@@yloop0	;=> more rows to copy

@@fixSrc2:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed2
@@fixDst2:
	call	NormalizeESDI
	jmp	SHORT @@DstFixed2

; Copy through transparency mask.

	EVEN

@@copyTrans:
@@yloop1:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh3:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot103		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh3										;+
	jmp	@@yendwh03										;+
@@ynot103:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh03		;{								;+
@@ydowh3:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop1:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw3		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip1	;=> don't copy this pixel
	mov	[es:di], al
@@skip1:
	inc	di
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit03		;		break;
@@xnotdraw3:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx3		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx3:			;	}							;+
@@xwh03:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh03		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip03	;=> don't copy this pixel
	mov	[es:di], al
@@skip03:
	inc	di
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh03		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit03		;			break;
				;	}							;+
@@xendwh03:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop1		;}								;+
@@xexit03:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst4	;=> normalize number
@@DstFixed4:
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot113		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot113:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk013		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk013:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh3
	ret			;	break;
@@yendwh03:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot123		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot123:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk023		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk023:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc4	;=> normalize number
@@SrcFixed4:
	jmp	@@yloop1	;=> more rows to copy

@@fixSrc4:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed4
@@fixDst4:
	call	NormalizeESDI
	jmp	SHORT @@DstFixed4

; Copy through pixel mask.

	EVEN
@@copyMask:
@@yloop2:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh4:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot104		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh4										;+
	jmp	@@yendwh04										;+
@@ynot104:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh04		;{								;+
@@ydowh4:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop2:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw4		;	{							;+
	mov	al, [ds:si]
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	stosb
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit04		;		break;
@@xnotdraw4:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx4		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx4:			;	}							;+
@@xwh04:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh04		;	{							;+
	mov	al, [ds:si]
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	stosb
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh04		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit04		;			break;
				;	}
@@xendwh04:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop2		;}								;+
@@xexit04:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst6	;=> normalize number
@@DstFixed6:
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot114		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot114:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk014		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk014:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh4
	ret			;	break;
@@yendwh04:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot124		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot124:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk024		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk024:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc6	;=> normalize number
@@SrcFixed6:
	jmp	@@yloop2	;=> more rows to copy

@@fixSrc6:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed6
@@fixDst6:
	call	NormalizeESDI
	jmp	SHORT @@DstFixed6

	ENDP	MCGA_ScaledMaskedCopyTransRect




	MPROC	MCGA_ScaledMaskedCopyXFlippedTransRect

	mov	bp, [cs:wDstWidth]	;save # of bytes to copy per row
	or	bp, bp
	jle	@@done	;=> width <= 0, no work
	cmp	[cs:wDstHeight], 0
	jle	@@done	;=> height <= 0, no work

	push	ax	;save srcX

	mov	ax, bx
	mul	[cs:sRowByte]	;AX = srcY * [cs:sRowByte]
	pop	bx
	add	ax, bx	;AX = srcY * [cs:sRowByte] + srcX
	add	si, ax
	jnc	@@doneMUL

	mov	ax, ds
	add	ax, 1000h
	mov	ds, ax
@@doneMUL:
	call	NormalizeDSSI

	mov	ax, [cs:wDstDivSrcSkipY]									;+
	mul	[cs:dRowByte]	;AX = (dstY/srcSkipY)*[cs:dRowByte]	;+
	add	ax, cx	;AX += (dstX / srcSkipX)			;+
	add	di, ax
	jnc	@@donMUL

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@donMUL:
	mov	ax, [cs:wDstModSrcSkipX]									;+
	cmp	ax, [cs:dstDupX]		;if (wDstX%wSrcSkipX >=wDstDupX);+
	jb	@@notIncX		;{								;+
	inc	di		;	pDst++;						;+
@@notIncX:			;}								;+
	mov	ax, [cs:wDstModSrcSkipY]									;+
	cmp	ax, [cs:dstDupY]		;if (wDstY%wSrcSkipY >=wDstDupY);+
	jb	@@notIncY		;{								;+
	add	di, [cs:dRowByte]		;	pDst += pDstBM->width;		;+
@@notIncY:			;}								;+
	call	NormalizeESDI

	add	di, [cs:wDstWidth]	;DI += [cs:wDstWidth]
	dec	di	;DI -= 1


	mov	ax, [cs:dRowByte]
	add	ax, [cs:wDstWidth]										;+
	mov	[cs:dskip], ax	;bytes to skip to next dest row

	mov	ax, [cs:wDstModSrcSkipY]									;+
	mov	[cs:wModY], ax		;wModY = wDstY % wSrcSkipY;		;+
	mov	ax, [cs:wDstHeight]										;+
	mov	[cs:wY], ax		;for (wY=0; wY<wSrcHeight; )	;+

	; Check which copy mode to use.

	mov	cl, [cs:_copyMode]
	and	cl, 011b
	jz	@@copyAll	;=> copy everything
	dec	cl
	jz	@@copyTrans	;=> use transparency mask
	dec	cl
	jz	@@copyMask	;=> use pixel mask

; Copy through pixel mask & transparency mask.

	EVEN
@@yloop3:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh1:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot101		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh1										;+
	jmp	@@yendwh01										;+
@@ynot101:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh01		;{								;+
@@ydowh1:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop3:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw0		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip0	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
@@skip0:
	dec	di
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit01		;		break;
@@xnotdraw0:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx0		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx0:			;	}							;+
@@xwh01:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh01		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip3	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
@@skip3:
	dec	di
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh01		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit01		;			break;
				;	}
@@xendwh01:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop3		;}								;+
@@xexit01:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst0	;=> normalize number
@@DstFixed0:
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot111		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot111:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk011		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk011:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh1
	ret			;	break;
@@yendwh01:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot121		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot121:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk021		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk021:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc0	;=> normalize number
@@SrcFixed0:
	jmp	@@yloop3	;=> more rows to copy
@@done:
	ret

@@fixSrc0:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed0
@@fixDst0:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed0

; Copy everything.

	EVEN
@@copyAll:
@@yloop0:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh0:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot10		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh0										;+
	jmp	@@yendwh0										;+
@@ynot10:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh0		;{								;+
@@ydowh0:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop0:			;{								;+
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw		;	{							;+
	mov	al, [ds:si]		;		*pDst = *psrc;			;+
	mov	[es:di], al
	dec	di
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit02		;		break;
@@xnotdraw:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx:			;	}							;+
@@xwh0:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh0		;	{							;+
	mov	al, [ds:si]		;		*pDst = *psrc;			;+
	mov	[es:di], al
	dec	di		;		pDst += wDstDx;
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh0		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit02		;			break;
				;	}
@@xendwh0:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop0		;}								;+
@@xexit02:
	add	di, [cs:dskip]	;move to next row in destination	;+
	js	@@fixDst2	;=> normalize number				;+
@@DstFixed2:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot11		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot11:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk01		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk01:				;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh0
	ret			;	break;
@@yendwh0:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot12		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot12:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk02		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk02:				;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc2	;=> normalize number
@@SrcFixed2:
	jmp	@@yloop0	;=> more rows to copy

@@fixSrc2:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed2
@@fixDst2:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed2

; Copy through transparency mask.

	EVEN
@@copyTrans:
@@yloop1:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh3:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot103		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh3										;+
	jmp	@@yendwh03										;+
@@ynot103:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh03		;{								;+
@@ydowh3:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop1:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw3		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip1	;=> don't copy this pixel
	mov	[es:di], al
@@skip1:
	dec	di
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit03		;		break;
@@xnotdraw3:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx3		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx3:			;	}							;+
@@xwh03:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh03		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip03	;=> don't copy this pixel
	mov	[es:di], al
@@skip03:
	dec	di
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh03		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit03		;			break;
				;	}
@@xendwh03:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop1		;}								;+
@@xexit03:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst4	;=> normalize number
@@DstFixed4:
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot113		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot113:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk013		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk013:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh3
	ret			;	break;
@@yendwh03:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot123		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot123:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk023		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk023:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc4	;=> normalize number
@@SrcFixed4:
	jmp	@@yloop1	;=> more rows to copy

@@fixSrc4:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed4
@@fixDst4:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed4

; Copy through pixel mask.

	EVEN
@@copyMask:
@@yloop2:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh4:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot104		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh4										;+
	jmp	@@yendwh04										;+
@@ynot104:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh04		;{								;+
@@ydowh4:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop2:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw4		;	{							;+
	mov	al, [ds:si]
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
	dec	di
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit04		;		break;
@@xnotdraw4:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx4		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx4:			;	}							;+
@@xwh04:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh04		;	{							;+
	mov	al, [ds:si]
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
	dec	di
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh04		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit04		;			break;
				;	}
@@xendwh04:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop2		;}								;+
@@xexit04:
	add	di, [cs:dskip]	;move to next row in destination
	js	@@fixDst6	;=> normalize number
@@DstFixed6:
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot114		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot114:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk014		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk014:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh4
	ret			;	break;
@@yendwh04:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot124		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot124:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk024		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk024:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc6	;=> normalize number
@@SrcFixed6:
	jmp	@@yloop2	;=> more rows to copy

@@fixSrc6:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed6
@@fixDst6:
	call	UnNormalizeESDI
	jmp	SHORT @@DstFixed6

	ENDP	MCGA_ScaledMaskedCopyXFlippedTransRect




	MPROC	MCGA_ScaledMaskedCopyYFlippedTransRect

	mov	bp, [cs:wDstWidth]	;save # of bytes to copy per row
	or	bp, bp
	jle	@@done	;=> width <= 0, no work
	cmp	[cs:wDstHeight], 0
	jle	@@done	;=> height <= 0, no work

	push	ax	;save srcX

	mov	ax, bx
	mul	[cs:sRowByte]	;AX = srcY * [cs:sRowByte]
	pop	bx
	add	ax, bx	;AX = srcY * [cs:sRowByte] + srcX
	add	si, ax
	jnc	@@doneMUL

	mov	ax, ds
	add	ax, 1000h
	mov	ds, ax
@@doneMUL:
	call	NormalizeDSSI

	mov	ax, [cs:wDstDivSrcSkipY]									;+
	mul	[cs:dRowByte]	;AX = (dstY/srcSkipY)*[cs:dRowByte]	;+
	add	ax, cx	;AX += (dstX / srcSkipX)			;+
	add	di, ax
	jnc	@@donMUL

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@donMUL:
	mov	ax, [cs:wDstHeight]
	dec	ax
	mul	[cs:dRowByte]	;AX = ([cs:wDstHeight]-1) * [cs:dRowByte]
	add	di, ax
	jnc	@@dontBump

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@dontBump:
	mov	ax, [cs:wDstModSrcSkipX]									;+
	cmp	ax, [cs:dstDupX]		;if (wDstX%wSrcSkipX >=wDstDupX);+
	jb	@@notIncX		;{								;+
	inc	di		;	pDst++;						;+
@@notIncX:			;}								;+
	mov	ax, [cs:wDstModSrcSkipY]									;+
	cmp	ax, [cs:dstDupY]		;if (wDstY%wSrcSkipY >=wDstDupY);+
	jb	@@notIncY		;{								;+
	add	di, [cs:dRowByte]		;	pDst += pDstBM->width;		;+
@@notIncY:			;}								;+
	call	UnNormalizeESDI

	mov	ax, [cs:dRowByte]
	add	ax, [cs:wDstWidth]										;+
	mov	[cs:dskip], ax	;bytes to skip to prev dest row

	mov	ax, [cs:wDstModSrcSkipY]									;+
	mov	[cs:wModY], ax		;wModY = wDstY % wSrcSkipY;		;+
	mov	ax, [cs:wDstHeight]										;+
	mov	[cs:wY], ax		;for (wY=0; wY<wSrcHeight; )	;+

	; Check which copy mode to use.

	mov	cl, [cs:_copyMode]
	and	cl, 011b
	jz	@@copyAll	;=> copy everything
	dec	cl
	jz	@@copyTrans	;=> use transparency mask
	dec	cl
	jz	@@copyMask	;=> use pixel mask

; Copy through pixel mask & transparency mask.

	EVEN
@@yloop3:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh1:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot101		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh1										;+
	jmp	@@yendwh01										;+
@@ynot101:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh01		;{								;+
@@ydowh1:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop3:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw0		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip0	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
@@skip0:
	inc	di
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit01		;		break;
@@xnotdraw0:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx0		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx0:			;	}							;+
@@xwh01:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh01		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip3	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
@@skip3:
	inc	di
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh01		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit01		;			break;
				;	}
@@xendwh01:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop3		;}								;+
@@xexit01:
	cmp	di, 1000
	jb	@@fixDst0	;=> unnormalize number
@@DstFixed0:
	sub	di, [cs:dskip]	;move to prev row in destination
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot111		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot111:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk011		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk011:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh1
	ret			;	break;
@@yendwh01:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot121		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot121:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk021		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk021:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc0	;=> normalize number
@@SrcFixed0:
	jmp	@@yloop3	;=> more rows to copy
@@done:
	ret

@@fixSrc0:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed0
@@fixDst0:
	call	UnNormalizeESDI
	jmp	@@DstFixed0

; Copy everything.

	EVEN
@@copyAll:
@@yloop0:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh0:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot10		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh0										;+
	jmp	@@yendwh0										;+
@@ynot10:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh0		;{								;+
@@ydowh0:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop0:			;{								;+
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw		;	{							;+
	mov	al, [ds:si]		;		*pDst = *psrc;			;+
	stosb			;		pDst += wDstDX;			;+
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit02		;		break;
@@xnotdraw:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx:			;	}							;+
@@xwh0:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh0		;	{							;+
	mov	al, [ds:si]		;		*pDst = *psrc;			;+
	stosb			;		pDst += wDstDx;
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh0		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit02		;			break;
				;	}							;+
@@xendwh0:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop0		;}								;+
@@xexit02:
	cmp	di, 1000
	jb	@@fixDst2	;=> unnormalize number
@@DstFixed2:
	sub	di, [cs:dskip]	;move to prev row in destination
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot11		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot11:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk01		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk01:				;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh0
	ret			;	break;
@@yendwh0:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot12		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot12:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk02		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk02:				;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc2	;=> normalize number
@@SrcFixed2:
	jmp	@@yloop0	;=> more rows to copy

@@fixSrc2:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed2
@@fixDst2:
	call	UnNormalizeESDI
	jmp	@@DstFixed2

; Copy through transparency mask.

	EVEN

@@copyTrans:
@@yloop1:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh3:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot103		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh3										;+
	jmp	@@yendwh03										;+
@@ynot103:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh03		;{								;+
@@ydowh3:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop1:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw3		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip1	;=> don't copy this pixel
	mov	[es:di], al
@@skip1:
	inc	di
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit03		;		break;
@@xnotdraw3:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx3		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx3:			;	}							;+
@@xwh03:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh03		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip03	;=> don't copy this pixel
	mov	[es:di], al
@@skip03:
	inc	di
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh03		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit03		;			break;
				;	}							;+
@@xendwh03:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop1		;}								;+
@@xexit03:
	cmp	di, 1000
	jb	@@fixDst4	;=> unnormalize number
@@DstFixed4:
	sub	di, [cs:dskip]	;move to prev row in destination
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot113		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot113:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk013		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk013:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh3
	ret			;	break;
@@yendwh03:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot123		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot123:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk023		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk023:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc4	;=> normalize number
@@SrcFixed4:
	jmp	@@yloop1	;=> more rows to copy

@@fixSrc4:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed4
@@fixDst4:
	call	UnNormalizeESDI
	jmp	@@DstFixed4

; Copy through pixel mask.

	EVEN
@@copyMask:
@@yloop2:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh4:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot104		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh4										;+
	jmp	@@yendwh04										;+
@@ynot104:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh04		;{								;+
@@ydowh4:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop2:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw4		;	{							;+
	mov	al, [ds:si]
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	stosb
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit04		;		break;
@@xnotdraw4:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx4		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx4:			;	}							;+
@@xwh04:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh04		;	{							;+
	mov	al, [ds:si]
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	stosb
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh04		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit04		;			break;
				;	}
@@xendwh04:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop2		;}								;+
@@xexit04:
	cmp	di, 1000
	jb	@@fixDst6	;=> unnormalize number
@@DstFixed6:
	sub	di, [cs:dskip]	;move to prev row in destination
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot114		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot114:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk014		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk014:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh4
	ret			;	break;
@@yendwh04:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot124		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot124:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk024		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk024:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc6	;=> normalize number
@@SrcFixed6:
	jmp	@@yloop2	;=> more rows to copy

@@fixSrc6:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed6
@@fixDst6:
	call	UnNormalizeESDI
	jmp	@@DstFixed6

	ENDP	MCGA_ScaledMaskedCopyYFlippedTransRect




	MPROC	MCGA_ScaledMaskedCopyXYFlippedTransRect

	mov	bp, [cs:wDstWidth]	;save # of bytes to copy per row
	or	bp, bp
	jle	@@done	;=> width <= 0, no work
	cmp	[cs:wDstHeight], 0
	jle	@@done	;=> height <= 0, no work

	push	ax	;save srcX

	mov	ax, bx
	mul	[cs:sRowByte]	;AX = srcY * [cs:sRowByte]
	pop	bx
	add	ax, bx	;AX = srcY * [cs:sRowByte] + srcX
	add	si, ax
	jnc	@@doneMUL

	mov	ax, ds
	add	ax, 1000h
	mov	ds, ax
@@doneMUL:
	call	NormalizeDSSI

	mov	ax, [cs:wDstDivSrcSkipY]									;+
	mul	[cs:dRowByte]	;AX = (dstY/srcSkipY)*[cs:dRowByte]	;+
	add	ax, cx	;AX += (dstX / srcSkipX)			;+
	add	ax, [cs:wDstWidth]	;AX = dstY * [cs:dRowByte] + dstX + [cs:wDstWidth]
	dec	ax	;AX = dstY * [cs:dRowByte] + dstX + [cs:wDstWidth]-1
	add	di, ax
	jnc	@@donMUL

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@donMUL:
	mov	ax, [cs:wDstHeight]
	dec	ax
	mul	[cs:dRowByte]	;AX = ([cs:wDstHeight]-1) * [cs:dRowByte]
	add	di, ax
	jnc	@@dontBump

	mov	ax, es
	add	ax, 1000h
	mov	es, ax
@@dontBump:
	mov	ax, [cs:wDstModSrcSkipX]									;+
	cmp	ax, [cs:dstDupX]		;if (wDstX%wSrcSkipX >=wDstDupX);+
	jb	@@notIncX		;{								;+
	inc	di		;	pDst++;						;+
@@notIncX:			;}								;+
	mov	ax, [cs:wDstModSrcSkipY]									;+
	cmp	ax, [cs:dstDupY]		;if (wDstY%wSrcSkipY >=wDstDupY);+
	jb	@@notIncY		;{								;+
	add	di, [cs:dRowByte]		;	pDst += pDstBM->width;		;+
@@notIncY:			;}								;+
	call	UnNormalizeESDI

	mov	ax, [cs:dRowByte]
	sub	ax, [cs:wDstWidth]										;+
	mov	[cs:dskip], ax	;bytes to skip to prev dest row

	mov	ax, [cs:wDstModSrcSkipY]									;+
	mov	[cs:wModY], ax		;wModY = wDstY % wSrcSkipY;		;+
	mov	ax, [cs:wDstHeight]										;+
	mov	[cs:wY], ax		;for (wY=0; wY<wSrcHeight; )	;+

	; Check which copy mode to use.

	mov	cl, [cs:_copyMode]
	and	cl, 011b
	jz	@@copyAll	;=> copy everything
	dec	cl
	jz	@@copyTrans	;=> use transparency mask
	dec	cl
	jz	@@copyMask	;=> use pixel mask

; Copy through pixel mask & transparency mask.

	EVEN
@@yloop3:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh1:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot101		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh1										;+
	jmp	@@yendwh01										;+
@@ynot101:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh01		;{								;+
@@ydowh1:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop3:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw0		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip0	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
@@skip0:
	dec	di
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit01		;		break;
@@xnotdraw0:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx0		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx0:			;	}							;+
@@xwh01:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh01		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip3	;=> don't copy this pixel
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
@@skip3:
	dec	di
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh01		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit01		;			break;
				;	}
@@xendwh01:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop3		;}								;+
@@xexit01:
	cmp	di, 1000
	jb	@@fixDst0	;=> unnormalize number
@@DstFixed0:
	sub	di, [cs:dskip]	;move to prev row in destination
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot111		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot111:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk011		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk011:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh1
	ret			;	break;
@@yendwh01:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot121		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot121:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk021		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk021:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc0	;=> normalize number
@@SrcFixed0:
	jmp	@@yloop3	;=> more rows to copy
@@done:
	ret

@@fixSrc0:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed0
@@fixDst0:
	call	UnNormalizeESDI
	jmp	@@DstFixed0

; Copy everything.

	EVEN
@@copyAll:
@@yloop0:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh0:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot10		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh0										;+
	jmp	@@yendwh0										;+
@@ynot10:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh0		;{								;+
@@ydowh0:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop0:			;{								;+
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw		;	{							;+
	mov	al, [ds:si]		;		*pDst = *psrc;			;+
	mov	[es:di], al
	dec	di		;		pDst += wDstDX;			;+
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit02		;		break;
@@xnotdraw:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx:			;	}							;+
@@xwh0:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh0		;	{							;+
	mov	al, [ds:si]		;		*pDst = *psrc;			;+
	mov	[es:di], al
	dec	di		;		pDst += wDstDX;			;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh0		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit02		;			break;
				;	}
@@xendwh0:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop0		;}								;+
@@xexit02:
	cmp	di, 1000
	jb	@@fixDst2	;=> unnormalize number
@@DstFixed2:
	sub	di, [cs:dskip]	;move to prev row in destination
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot11		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot11:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk01		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk01:				;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh0
	ret			;	break;
@@yendwh0:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot12		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot12:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk02		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk02:				;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc2	;=> normalize number
@@SrcFixed2:
	jmp	@@yloop0	;=> more rows to copy

@@fixSrc2:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed2
@@fixDst2:
	call	UnNormalizeESDI
	jmp	@@DstFixed2

; Copy through transparency mask.

	EVEN
@@copyTrans:
@@yloop1:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh3:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot103		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh3										;+
	jmp	@@yendwh03										;+
@@ynot103:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh03		;{								;+
@@ydowh3:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop1:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw3		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip1	;=> don't copy this pixel
	mov	[es:di], al
@@skip1:
	dec	di
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit03		;		break;
@@xnotdraw3:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx3		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx3:			;	}							;+
@@xwh03:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh03		;	{							;+
	mov	al, [ds:si]
	cmp	al, [BYTE PTR cs:pixTrans]
	je	@@skip03	;=> don't copy this pixel
	mov	[es:di], al
@@skip03:
	dec	di
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh03		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit03		;			break;
				;	}
@@xendwh03:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop1		;}								;+
@@xexit03:
	cmp	di, 1000
	jb	@@fixDst4	;=> unnormalize number
@@DstFixed4:
	sub	di, [cs:dskip]	;move to prev row in destination
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot113		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot113:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk013		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk013:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh3
	ret			;	break;
@@yendwh03:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot123		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot123:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk023		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk023:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc4	;=> normalize number
@@SrcFixed4:
	jmp	@@yloop1	;=> more rows to copy

@@fixSrc4:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed4
@@fixDst4:
	call	UnNormalizeESDI
	jmp	@@DstFixed4

; Copy through pixel mask.

	EVEN
@@copyMask:
@@yloop2:
	sub	ah, ah		;fFirst = TRUE;					;+
	mov	[cs:SIsave], si		;pSrcSave = psrc;				;+
@@ywh4:												;+
	mov	cx, [cs:wModY]										;+
	or	ah, ah		;while ((fFirst &&				;+
	jnz	@@ynot104		;								;+
	cmp	cx, [cs:dstDupY]		;		(wModY < wDstDupY)) ||	;+
	jb	@@ydowh4										;+
	jmp	@@yendwh04										;+
@@ynot104:			;		(!fFirst &&				;+
	cmp	cx, [cs:srcSkipY]		;		(wModY >= wSrcSkipY)) )	;+
	jb	@@yendwh04		;{								;+
@@ydowh4:											;+
	mov	dx, [cs:wDstModSrcSkipX]	;wModX = wDstX % wSrcSkipX;		;+
	mov	cx, bp		;for (wX=0; wX<wSrcWidth; )		;+
@@xloop2:
	cmp	dx, [cs:dstDupX]		;	if (wModX < wDstDupX)		;+
	jae	@@xnotdraw4		;	{							;+
	mov	al, [ds:si]
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
	dec	di
	dec	cx		;	if (++wX >= wDstWidth)
	je	@@xexit04		;		break;
@@xnotdraw4:			;	}							;+
	add	dx, [cs:dstDupX]		;	wModX += wDstDupX			;+
												;+
	cmp	dx, [cs:srcSkipX]		;	if (wModX >= wSrcSkipX)		;+
	jb	@@wmltssx4		;	{							;+
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
@@wmltssx4:			;	}							;+
@@xwh04:												;+
	cmp	dx, [cs:srcSkipX]		;	while (wModX >= wSrcSkipX)	;+
	jb	@@xendwh04		;	{							;+
	mov	al, [ds:si]
	and	al, [cs:_pixelAND]
	or	al, [cs:_pixelOR]
	mov	[es:di], al
	dec	di
	sub	dx, [cs:srcSkipX]		;		wModX -= wSrcSkipX;		;+
	loop	@@xwh04		;		if (++wX >= wDstWidth)
	jmp	SHORT @@xexit04		;			break;
				;	}
@@xendwh04:											;+
	inc	si		;	psrc++;						;+
	jmp	@@xloop2		;}								;+
@@xexit04:
	cmp	di, 1000
	jb	@@fixDst6	;=> unnormalize number
@@DstFixed6:
	sub	di, [cs:dskip]	;move to prev row in destination
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot114		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
	inc	ah		;	fFirst = FALSE				;+
@@ynot114:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk014		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk014:			;}								;+
	mov	si, [cs:SIsave]		;psrc = pSrcSave;				;+
	dec	[cs:wY]		;if ((++wY >= wDstHeight)
	jne	@@ywh4
	ret			;	break;
@@yendwh04:											;+
	or	ah, ah		;if (fFirst)					;+
	jnz	@@ynot124		;{								;+
	mov	cx, [cs:dstDupY]										;+
	add	[cs:wModY], cx		;	wModY += wDstDupY;			;+
@@ynot124:			;}								;+
	mov	cx, [cs:wModY]										;+
	cmp	cx, [cs:srcSkipY]		;if (wModY >= wSrcSkipY)		;+
	jb	@@ysk024		;{								;+
	mov	cx, [cs:SrcSkipY]										;+
	sub	[cs:wModY], cx		;	wModY -= wSrcSkipY;			;+
@@ysk024:			;}								;+
	add	si, [cs:sRowByte]		;pSrc += pSrcBM->width			;+
	js	@@fixSrc6	;=> normalize number
@@SrcFixed6:
	jmp	@@yloop2	;=> more rows to copy

@@fixSrc6:
	call	NormalizeDSSI
	jmp	SHORT @@SrcFixed6
@@fixDst6:
	call	UnNormalizeESDI
	jmp	@@DstFixed6

	ENDP	MCGA_ScaledMaskedCopyXYFlippedTransRect




;=====================================================================
;**
;** _MCGA_ScaledMaskedCopyTransRect
;**
;** PURPOSE
;**	C glue for copy rect as above.  C Prototype is
;**
;**	MCGA_ScaledMaskedCopyTransRect (
;**		ByteMap	*src,
;**		WORD	 fromx,
;**		WORD	 fromy,
;**		ByteMap	*dest,
;**		LONG	 tox,
;**		LONG	 toy,
;**		WORD	 wDstWidth,
;**		WORD	 wDstHeight,
;**		WORD	 wSrcSkipX,
;**		WORD	 wDstDupX,
;**		WORD	 wSrcSkipY,
;**		WORD	 wDstDupY
;**		)
;**
;**	This routine looks at the global variable _copyMode:
;**	_copyMode bit 6 set: x flip tile
;**	_copyMode bit 5 set: y flip tile
;**	Thus, if bit 5 is set, MCGA_ScaledMaskedCopyYFlippedTransRect is called,
;**	if bit 6 is set, MCGA_ScaledMaskedCopyXFlippedTransRect is called,
;**	if bit 5 & 6 are set, MCGA_ScaledMaskedCopyXYFlippedTransRect is called,
;**	else MCGA_ScaledMaskedCopyTransRect is called.
;**
;** INPUT
;**	
;**
;** REGISTER USED
;**	
;**
;** EFFECTS
;**	
;**
;** RETURN VALUE
;**	
;**
;** LIMITS/BUGS
;**	
;**
;** HISTORY
;**	
;**

	EVEN

	PUBLIC	_MCGA_ScaledMaskedCopyTransRect
	MPROC	_MCGA_ScaledMaskedCopyTransRect
	ARG	sBMoff:WORD,sBMseg:WORD,FromX:WORD,FromY:WORD,dBMoff:WORD,dBMseg:WORD,ToX:DWORD,ToY:DWORD,	\
		GWidth:WORD,Height:WORD,wSrcSkipX:WORD,wDstDupX:WORD,wSrcSkipY:WORD,wDstDupY:WORD

	push	bp
	mov	bp, sp

	push	ds
	push	di
	push	si

	cld
	mov	ax, [wSrcSkipX]										;+
	mov	[cs:srcSkipX], ax										;+
	mov	ax, [wDstDupX]										;+
	mov	[cs:dstDupX], ax										;+
	mov	ax, [wSrcSkipY]										;+
	mov	[cs:srcSkipY], ax										;+
	mov	ax, [wDstDupY]										;+
	mov	[cs:dstDupY], ax										;+

	mov	ax, [GWidth]
	mov	[cs:wDstWidth], ax
	mov	ax, [Height]
	mov	[cs:wDstHeight], ax

	lds	si,[dword ptr dBMoff]	; Get Pointer to Dest ByteMap
	mov	ax,[(ByteMap ptr ds:si).Width]
  	mov	[cs:dRowByte],ax	; Set width of Dest ByteMap
	les	di, [(ByteMap ptr ds:si).gfxData]	;DCC Get Off, Seg of Dest ByteMap

	lds	si,[dword ptr sBMoff]	; Get Pointer to Src ByteMap

	mov	ax, [(ByteMap ptr ds:si).TColor]	;get transparent color
	mov	[cs:pixTrans], ax

	mov	ax,[(ByteMap ptr ds:si).Width]
  	mov	[cs:sRowByte],ax	; Set width of Src ByteMap
	lds	si, [(ByteMap ptr ds:si).gfxData]	;DCC Get Off, Seg of Src ByteMap

	mov	dx, [WORD PTR ToX+2]										;+
	mov	ax, [WORD PTR ToX]										;+
	idiv	[wSrcSkipX]	;DX = ToX % [cs:srcSkipX]			;+
	mov	[cs:wDstModSrcSkipX], dx									;+
	mov	cx, ax										;+
												;+
	mov	dx, [WORD PTR ToY+2]										;+
	mov	ax, [WORD PTR ToY]										;+
	idiv	[wSrcSkipY]	;DX = dstY % [cs:srcSkipY]			;+
	mov	[cs:wDstModSrcSkipY], dx									;+
	mov	[cs:wDstDivSrcSkipY], ax									;+
												;+
	mov	ax, [FromX]
	mov	bx, [FromY]

	mov	bp, [WORD PTR cs:_copyMode]
	and	bp, 01100000b

	cmp	bp, 00100000b
	jne	@@not01

	call	MCGA_ScaledMaskedCopyYFlippedTransRect
	jmp	SHORT @@done
@@not01:
	cmp	bp, 01000000b
	jne	@@not10

	call	MCGA_ScaledMaskedCopyXFlippedTransRect
	jmp	SHORT @@done
@@not10:
	cmp	bp, 01100000b
	jne	@@not11

	call	MCGA_ScaledMaskedCopyXYFlippedTransRect
	jmp	SHORT @@done
@@not11:
	call	MCGA_ScaledMaskedCopyTransRect
@@done:
	pop	si
	pop	di
	pop	ds

	pop	bp
	ret
	ENDP	_MCGA_ScaledMaskedCopyTransRect

	ENDS
	END
