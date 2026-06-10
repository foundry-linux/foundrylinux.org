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
;** SHOWCPU.ASM
;**
;** PROGRAMMER : Dan Chang
;**    VERSION : 00.000
;**    CREATED : 07/28/94
;**   MODIFIED : 07/28/94
;**       TABS : 09 17
;**
;** DESCRIPTION
;**	Print error message, CS:IP, registers, and instruction stream.
;**
;** HISTORY
;**	07/28/94 Thursday (dcc) - created.
;**

	INCLUDE	SWITCH1.INC

	IF	DEBUG

	extrn	_printf:far
	public	__ShowCPU

	assume	cs:SHOWCPU_TEXT, ds:SHOWCPU_DATA

SHOWCPU_DATA	segment word public 'FAR_DATA'

szErr1	label	byte
	db	' at %04X:%04X',10
	db	0

szErr2	label	byte
	db	'AX=%04X BX=%04X CX=%04X DX=%04X SI=%04X DI=%04X BP=%04X',10
	db	0

szErr3	label	byte
	db	'DS=%04X ES=%04X SS=%04X SP=%04X Flags=%04X',10
	db	0

szErr4	label	byte
	db	'Instruction: %02X %02X %02X %02X %02X %02X %02X'
	db	' %02X %02X %02X %02X %02X %02X %02X %02X',10
	db	0

SHOWCPU_DATA	ends

SHOWCPU_TEXT	segment byte public 'CODE'

	assume	cs:SHOWCPU_TEXT

;=====================================================================
;**
;** __ShowCPU
;**
;** PURPOSE
;**	Print error message, CS:IP, registers, and instruction stream.
;**
;** INPUT
;**	[bp+36]		: Flags
;**	[bp+34]		: CS
;**	[bp+32]		: IP

;**	[bp+30]		: SS
;**	[bp+28]		: ES
;**	[bp+26]		: DS

;**	[bp+24]		: BP
;**	[bp+22]		: DI
;**	[bp+20]		: SI
;**	[bp+18]		: DX
;**	[bp+16]		: CX
;**	[bp+14]		: BX
;**	[bp+12]		: AX

;**	[bp+10]		: SP

;**	[bp+08]		: segment of error message
;**	[bp+06]		: offset  of error message
;**
;** ASSUMES
;**	
;**
;** REGISTER CHANGED
;**	
;**
;** RETURN VALUE
;**	
;**
;** LIMITS/BUGS
;**	
;**
;** HISTORY
;**	07/28/94 Thursday (dcc) - created.
;**


__ShowCPU	proc	far

	push	bp
	mov	bp, sp

	; Set text mode B4 printing error message

	mov	ax, 03H
	int	10H

	; print error message

	push	[bp+08]
	push	[bp+06]
	call	far ptr _printf
	add	sp, 4

	; print CS:IP

	push	[bp+32]
	push	[bp+34]
	mov	ax, seg szErr1
	push	ax
	mov	ax, offset szErr1
	push	ax
	call	far ptr _printf
	add	sp, 8

	; Print AX, BX, CX, DX, SI, DI, BP.
	; Use the values [bp+12]..[bp+24] (AX,BX,CX,DX,SI,DI,BP) on stack.

	push	[bp+24]
	push	[bp+22]
	push	[bp+20]
	push	[bp+18]
	push	[bp+16]
	push	[bp+14]
	push	[bp+12]
	mov	ax, seg szErr2
	push	ax
	mov	ax, offset szErr2
	push	ax
	call	far ptr _printf
	add	sp, 18

	; Print DS, ES, SS, SP, Flags.

	push	[bp+36]			; Flags
	push	[bp+10]			; SP
	push	[bp+30]			; SS
	push	[bp+28]			; ES
	push	[bp+26]			; DS

	mov	ax, seg szErr3
	push	ax
	mov	ax, offset szErr3
	push	ax
	call	far ptr _printf
	add	sp, 14

	; Print bytes at instruction pointer CS:IP.

	mov	ax, [bp+34]		; CS
	mov	ds, ax

	mov	si, [bp+32]		; IP
	add	si, 14

	std
	sub	ax, ax
	mov	cx, 15
@again:
	lodsb
	push	ax
	loop	@again

	mov	ax, seg szErr4
	push	ax
	mov	ax, offset szErr4
	push	ax
	call	far ptr _printf
	add	sp, 34

	pop	bp
	ret

__ShowCPU	endp


SHOWCPU_TEXT	ends

	ENDIF	;DEBUG

	end

