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
;** NULLPTR.ASM
;**
;** PROGRAMMER : Dan Chang
;**    VERSION : 00.000
;**    CREATED : 10/01/92
;**   MODIFIED : 07/28/94
;**       TABS : 09 17
;**
;** DESCRIPTION
;**
;**    This module contains the function void InstallCheckPointer(void);
;** call it to set up an interrupt 60H handler.
;**
;** HISTORY
;**	10/03/92 Saturday (dcc) - initial version created.
;**	03/18/94 Friday (dcc) - check for address above B800H as well
;**	07/20/94 Wednesday (dcc) - set text mode before printing error message
;**	07/20/94 Wednesday (dcc) - remove check for address > B800H (so EMS works)
;**	07/28/94 Thursday (dcc) - revised to call __ShowCPU
;**

	INCLUDE	SWITCH1.INC

	IF	DEBUG

	extrn	__ShowCPU:far

	public	_InstallCheckPointer

	assume	cs:NULLPTR_TEXT, ds:NULLPTR_DATA

NULLPTR_DATA	segment word public 'FAR_DATA'

errmsg1	label	byte
	db	'NULL pointer reference'
	db	0

NULLPTR_DATA	ends

NULLPTR_TEXT	segment byte public 'CODE'

	assume	cs:NULLPTR_TEXT

; This routine gets installed as an int 60H handler.
; It aborts the program with an error message
; if segment register ES equals 0.

__CheckPointer	proc	far

	push	ax
	mov	ax, es
	or	ax, ax
	pop	ax
	je	@ESEquals0		; => ES == 0, show error message
	iret

@ESEquals0:
					; Flags
					; CS
					; IP

	push	ss
	push	es
	push	ds
	push	bp
	push	di
	push	si
	push	dx
	push	cx
	push	bx
	push	ax

	mov	ax, bp
	add	ax, 24
	push	ax			; SP

	mov	ax, [bp-14]
	sub	ax, 2
	mov	[bp-14], ax		; [bp-14] = IP-2

	mov	ax, seg errmsg1
	push	ax			; segment of error message
	mov	ax, offset errmsg1
	push	ax			; offset  of error message
	call	far ptr __ShowCPU
	add	sp, 32

@forever:
	jmp	@forever		; Don't go anywhere

__CheckPointer	endp

_InstallCheckPointer	proc	far

	push	ds

	mov	ax, cs
	mov	ds, ax
	mov	dx, offset __CheckPointer
	mov	ax, 02560h		; Install new int 60H handler
	int	21h

	pop	ds
	ret

_InstallCheckPointer	endp

NULLPTR_TEXT	ends

	ENDIF	;DEBUG

	end

