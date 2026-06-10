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
;** DIV0TRAP.ASM
;**
;** PROGRAMMER : Dan Chang
;**    VERSION : 00.000
;**    CREATED : 07/28/94
;**   MODIFIED : 07/28/94
;**       TABS : 09 17
;**
;** DESCRIPTION
;**	Borland C real-mode huge model divide by 0 interrupt handler.
;**	Print the CS:IP, registers when a divide by 0 occurs or
;**	when a illegal instruction is executed.
;**
;**	If you are using exit function support of EGGS, you must call
;**	AddExitFunc() at least once BEFORE you call InstallDivide0Trap().
;**
;**	NOTE: BUGBUG: hardcoded to work with the Borland C 4.02 handler.
;**
;** HISTORY
;**	07/28/94 Thursday (dcc) - created.
;**

	INCLUDE	SWITCH1.INC

	IF	DEBUG

	extrn	_signal:far
	extrn	__ShowCPU:far

	public	_InstallDivide0Trap

SIGFPE	equ	8
SIGILL	equ	4

	assume	cs:DIV0TRAP_TEXT, ds:DIV0TRAP_DATA

DIV0TRAP_DATA	segment word public 'FAR_DATA'

szErr0	label	byte
	db	'Divide by 0 error'
	db	0

szErr1	label	byte
	db	'Illegal instruction'
	db	0

DIV0TRAP_DATA	ends

DIV0TRAP_TEXT	segment byte public 'CODE'

	assume	cs:DIV0TRAP_TEXT

;=====================================================================
;**
;** __Div0Handler
;**
;** PURPOSE
;**	This routine is called by Borland C's int 0 / int 6 handler.
;**
;** INPUT
;**	[bp+06]:	signal value, one of two values:
;**			SIGFPE (8): floating point trap (divide by 0 error)
;**			SIGILL (4): illegal instruction
;**
;**	We know that the following is on the stack (thanks to Borland C).
;**
;**	[bp+40]:	Flags
;**	[bp+38]:	CS
;**	[bp+36]:	IP
;**	[bp+34]:	AX
;**	[bp+32]:	BX
;**	[bp+30]:	CX
;**	[bp+28]:	DX
;**	[bp+26]:	ES
;**	[bp+24]:	DS
;**	[bp+22]:	SI
;**	[bp+20]:	DI
;**	[bp+18]:	BP
;**
;** ASSUMES
;**	
;**
;** REGISTER CHANGED
;**	
;**
;** RETURN VALUE
;**	NONE.
;**
;** LIMITS/BUGS
;**	
;**
;** HISTORY
;**	07/28/94 Thursday (dcc) - created.
;**

__Div0Handler	proc	far

	push	bp
	mov	bp, sp
	
	push	[bp+40]			; Flags
	push	[bp+38]			; CS
	push	[bp+36]			; IP

	mov	ax, ss			; SS
	push	ax

	push	[bp+26]			; ES
	push	[bp+24]			; DS

	push	[bp+18]			; BP
	push	[bp+20]			; DI
	push	[bp+22]			; SI
	push	[bp+28]			; DX
	push	[bp+30]			; CX
	push	[bp+32]			; BX
	push	[bp+34]			; AX

	mov	ax, bp
	add	ax, 42
	push	ax			; SP

	mov	ax, [bp+06]
	cmp	ax, SIGFPE
	jz	@@IsFPE

	; Illegal instruction error

	mov	ax, seg szErr1
	push	ax			; segment of error message
	mov	ax, offset szErr1
	push	ax			; offset  of error message
	call	far ptr __ShowCPU
	add	sp, 32
	jmp	SHORT @@Shown
@@IsFPE:
	; Divide by 0 error

	mov	ax, seg szErr0
	push	ax			; segment of error message
	mov	ax, offset szErr0
	push	ax			; offset  of error message
	call	far ptr __ShowCPU
	add	sp, 32
@@Shown:
	pop	bp
	ret

__Div0Handler	endp

;=====================================================================
;**
;** _InstallDivide0Trap
;**
;** PURPOSE
;**	Install the routine to print the register on an divide
;**	by 0 or illegal instruction exception.
;**
;** INPUT
;**	NONE.
;**
;** ASSUMES
;**	
;**
;** REGISTER CHANGED
;**	
;**
;** RETURN VALUE
;**	NONE.
;**
;** LIMITS/BUGS
;**	
;**
;** HISTORY
;**	07/28/94 Thursday (dcc) - created.
;**


_InstallDivide0Trap	proc	far

	push	bp
	mov	bp, sp

	mov	ax, seg __Div0Handler
	push	ax
	mov	ax, offset __Div0Handler
	push	ax
	mov	ax, SIGFPE
	push	ax
	call	far ptr _signal
	add	sp, 6

	pop	bp
	ret

_InstallDivide0Trap	endp

DIV0TRAP_TEXT	ends

	ENDIF	;DEBUG

	end

