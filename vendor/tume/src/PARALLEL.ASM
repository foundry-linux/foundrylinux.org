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
;** PARALLEL.ASM
;**
;** PROGRAMMER : Dan Chang
;**    VERSION : 00.000
;**    CREATED : 12/08/92
;**   MODIFIED : 07/27/94
;**       TABS : 09 17
;**
;** DESCRIPTION
;**	Routines to read and write a byte from the parallel port
;**	with handshaking.
;**
;** HISTORY
;**	04/13/93 Tuesday (dcc) - go paranoid - read everything twice
;**	04/22/93 Thursday (dcc) - read some things more than twice!
;**

	IDEAL

	INCLUDE	"SWITCH2.INC"

	IF	dvpPARALLEL

bDATASTROBE	equ	0
bERROR		equ	3
bONLINE		equ	4
bOUTOFPAPER	equ	5
bACKNOWLEDGE	equ	6
bBUSY		equ	7

fDATASTROBE	equ	(1 SHL bDATASTROBE)
fERROR		equ	(1 SHL bERROR)
fBUSY		equ	(1 SHL bBUSY)

	MODEL	HUGE

	public	_ParallelPortAddress
	public	_ParallelPrevControl
	public	_ParallelSendByte
	public	_ParallelGetByte

	DATASEG

_ParallelPortAddress	dw	0378h
_ParallelPrevControl	db	0

	CODESEG

	PROC	_ParallelSendByte
	ARG	ub:BYTE

	push	bp
	mov	bp, sp
	push	ds

	mov	ax, seg _ParallelPortAddress
	mov	ds, ax

	mov	dx, [_ParallelPortAddress]

	inc	dx

@@Wait4_BUSY_:
	in	al, dx
	and	al, fBUSY
	jz	@@Wait4_BUSY_		;=> BUSY line still high

	in	al, dx
	and	al, fBUSY
	jz	@@Wait4_BUSY_		;=> BUSY line still high

	in	al, dx
	and	al, fBUSY
	jz	@@Wait4_BUSY_		;=> BUSY line still high

	in	al, dx
	and	al, fBUSY
	jz	@@Wait4_BUSY_		;=> BUSY line still high

	mov	al, [ub]
	dec	dx
	out	dx, al			;DATA: send data to output port

	add	dx, 2
	mov	al, [_ParallelPrevControl]
	or	al, fDATASTROBE		;Set D0 hi (/DATASTROBE lo)
	out	dx, al			;CONTROL: set DATASTROBE low

	dec	dx
@@Wait4BUSY:
	in	al, dx
	and	al, fBUSY
	jnz	@@Wait4BUSY		;=> BUSY line still low

	in	al, dx
	and	al, fBUSY
	jnz	@@Wait4BUSY		;=> BUSY line still low

	mov	al, 0FFh
	dec	dx
	out	dx, al			;DATA: set data bit 0 high

	add	dx, 2
	mov	al, [_ParallelPrevControl]
	and	al, 0FFh-fDATASTROBE	;Set D0 lo (/DATASTROBE hi)
	out	dx, al			;CONTROL: set /DATASTROBE high

	mov	ax, 1

	pop	ds
	pop	bp
	retf
	ENDP	_ParallelSendByte

	PROC	_ParallelGetNibble

	push	bp
	mov	bp, sp
	push	ds

	mov	ax, seg _ParallelPortAddress
	mov	ds, ax

	mov	dx, [_ParallelPortAddress]

	;(1) Remove busy. Set /DATASTROBE high, data bit 0 low.

	sub	al, al
	out	dx, al			;DATA: set data bit 0 low

	add	dx, 2
	mov	al, [_ParallelPrevControl]
	and	al, 0FFh-fDATASTROBE	;Set D0 lo (/DATASTROBE hi)
	out	dx, al			;CONTROL: set /DATASTROBE HI

	;(2) Wait for data strobe (pin 15 == LO, bit 3 of STATUS == 0).

	dec	dx
@@Wait4Strobe:
	in	al, dx
	test	al, fERROR
	jnz	@@Wait4Strobe		;=> pin 15 still HI

	mov	ah, al
	in	al, dx
	cmp	ah, al
	jnz	@@Wait4Strobe		;=> 2nd read not same

	;(3) Read nibble. At this point, bits 4..7 contains valid data.

	xor	al, 10000000b		;however, bit 7 sense is flipped
	mov	ah, al			;save nibble in AH

	;(4) Acknowledge data. Set pin 2 HI (set bit 0 of DATA to 1).

	mov	al, 0FFh
	dec	dx
	out	dx, al			;DATA: set data bit 0 high

	;(5) Wait for sender acknowledgement (pin 15 == HI, bit 3 of STATUS == 1).

	inc	dx
@@Wait4Ack:
	in	al, dx
	test	al, fERROR
	jz	@@Wait4Ack		;=> pin 15 still LO

	mov	al, ah			;restore nibble from AH

	pop	ds
	pop	bp
	retf
	ENDP	_ParallelGetNibble

	PROC	_ParallelGetByte

	call	_ParallelGetNibble
	shr	ax, 4
	and	ax, 1111b
	mov	bx, ax
	call	_ParallelGetNibble
	and	ax, 11110000b
	or	al, bl
					; ASSERT: AH == 0
	retf
	ENDP	_ParallelGetByte
	ENDS

	ENDIF	;dvpPARALLEL

	END
