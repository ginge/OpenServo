; DIGITAL SERVO CONTROLLER					Interrupt Service Rountines
; ---------------------------------------------------------------------------------------
; http://www.colinmackenzie.net
;
; CVS REPOSITORY: (get cvs at http://www.cvshome.org/)
;   The superservo source files are held in a cvs repository. CVS is the 
;   Concurrent Versions System, the dominant open-source network-transparent 
;   version control system.  
;
;   to login to cvs:
;   cvs -d :pserver:username@cvs.colinmackenzie.net:/var/lib/cvs login
;
;   to checkout servo controller:
;   cvs -d :pserver:username@cvs.colinmackenzie.net:/var/lib/cvs checkout superservo
;
; DESCRIPTION:
;   This file contains the ISR jump table macros.
;
; HISTORY:
;	Created Dec 28th, 2005	Colin MacKenzie
;
; KNOWN BUGS:
;	None.
;
; WARNINGS:
;	None.
;
; ROUTINES:
;
; MACROS:
;    ISR_CONTEXT_SAVE			-- save cpu state for isr entry
;    ISR_CONTEXT_RESTORE		-- restore cpu state to pre-isr state
;    isr_jumpif					-- jump to jump vector if give INT flag is set


; This macro simplifies the look of the ISR jump table
isr_jumpif macro INT_REG, INT_FLAG, JUMP_VECTOR
	btfsc	INT_REG, INT_FLAG
	goto	JUMP_VECTOR
	endm

ISR_CONTEXT_SAVE macro
	bcf		INTCON, GIE		; disable interrupts while in service routine
	MOVWF 	ISR_W 			; Copy W to TEMP register
	SWAPF 	STATUS,W 		; Swap status to be saved into W
	CLRF 	STATUS 			; bank 0, regardless of current bank, Clears IRP,RP1,RP0
	MOVWF 	ISR_STATUS	 	; Save status to bank zero STATUS_TEMP register
	MOVF 	PCLATH, W 		; Only required if using pages 1, 2 and/or 3
	MOVWF 	ISR_PCLATH 		; Save PCLATH into W
	CLRF 	PCLATH 			; Page zero, regardless of current page


;	movwf	ISR_W			; save off current W register contents
;	movf	STATUS,w		; move status register into W register
;	movwf	ISR_STATUS		; save off contents of STATUS register
;	movf	PCLATH,w		; move pclath register into w register
;	movwf	ISR_PCLATH		; save off contents of PCLATH register
	endm

ISR_CONTEXT_RESTORE macro
	movf	ISR_PCLATH,w	; retrieve copy of PCLATH register
	movwf	PCLATH			; restore pre-isr PCLATH register contents
	swapf	ISR_STATUS,w	; retrieve copy of STATUS register (swap back to orig state)
	movwf	STATUS			; restore pre-isr STATUS register contents
	swapf	ISR_W,f			; swapf is used because it doesnt affect flags STATUS flags
	swapf	ISR_W,w			; restore pre-isr W register contents
	endm




